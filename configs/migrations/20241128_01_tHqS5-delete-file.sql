CREATE OR REPLACE FUNCTION permission_app.DeleteFileByNameAndUser(
    file_name TEXT,
    username TEXT
)
RETURNS TEXT
LANGUAGE plpgsql
AS $$
DECLARE
    file_id INT;                -- ID файла
    owner_id INT;               -- ID владельца файла
    owner_group_id INT;         -- ID группы-владельца файла
    requester_id INT;           -- ID пользователя, выполняющего запрос
    is_root BOOLEAN;            -- Флаг root-прав пользователя
    belongs_to_group BOOLEAN;   -- Флаг, принадлежит ли пользователь группе-владельцу
BEGIN
    SELECT user_id, root INTO requester_id, is_root
    FROM permission_app.users
    WHERE login = username;

    IF requester_id IS NULL THEN
        RETURN 'Denied';
    END IF;

    SELECT n.file_id, p.user_id, p.group_id INTO file_id, owner_id, owner_group_id
    FROM permission_app.nodes n
    LEFT JOIN permission_app.permissions p ON n.file_id = p.node_id
    WHERE n.name = file_name AND n.type = 'FILE';

    IF file_id IS NULL THEN
        RETURN 'Not found';
    END IF;

    SELECT EXISTS(
        SELECT 1
        FROM permission_app.user_to_group
        WHERE user_id = requester_id AND group_id = owner_group_id
    ) INTO belongs_to_group;

    IF NOT (requester_id = owner_id OR is_root OR belongs_to_group) THEN
        RETURN 'Denied';
    END IF;

    DELETE FROM permission_app.nodes
    WHERE file_id = file_id;

    RETURN 'Success';
END;
$$;

GRANT EXECUTE ON FUNCTION permission_app.DeleteFileByNameAndUser(TEXT, TEXT) TO app_user;
