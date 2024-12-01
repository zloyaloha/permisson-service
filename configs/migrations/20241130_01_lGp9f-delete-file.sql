-- Delete file
-- depends: 20241124_01_81y4a-add-directory

CREATE OR REPLACE FUNCTION permission_app.DeleteFileByNameAndUser(
    username TEXT,
    directory_name TEXT
)
RETURNS TEXT
LANGUAGE plpgsql
AS $$
DECLARE
    directory_id INT;          -- ID директории
    owner_id INT;              -- ID владельца директории
    owner_group_id INT;        -- ID группы-владельца директории
    requester_id INT;          -- ID пользователя, выполняющего запрос
    is_root BOOLEAN;           -- Флаг root-прав пользователя
    belongs_to_group BOOLEAN;  -- Флаг, принадлежит ли пользователь группе-владельцу
BEGIN
    -- Получаем ID пользователя и его root-права
    SELECT user_id, root INTO requester_id, is_root
    FROM permission_app.users
    WHERE login = username;

    IF requester_id IS NULL THEN
        RETURN 'User not found';
    END IF;

    -- Получаем ID директории и владельцев
    SELECT n.file_id, p.user_id, p.group_id INTO directory_id, owner_id, owner_group_id
    FROM permission_app.nodes n
    LEFT JOIN permission_app.permissions p ON n.file_id = p.node_id
    WHERE n.name = directory_name;

    IF directory_id IS NULL THEN
        RETURN 'Not found';
    END IF;

    -- Проверяем, принадлежит ли пользователь группе-владельцу
    SELECT EXISTS(
        SELECT 1
        FROM permission_app.user_to_group
        WHERE user_id = requester_id AND group_id = owner_group_id
    ) INTO belongs_to_group;

    -- Проверка прав пользователя
    IF NOT (requester_id = owner_id OR is_root OR belongs_to_group) THEN
        RETURN 'Denied';
    END IF;

    -- Удаление директории и её содержимого рекурсивно
    DELETE FROM permission_app.nodes
    WHERE file_id IN (
        WITH RECURSIVE directories_to_delete AS (
            SELECT file_id
            FROM permission_app.nodes
            WHERE file_id = directory_id
            UNION ALL
            SELECT n.file_id
            FROM permission_app.nodes n
            INNER JOIN directories_to_delete dtd ON n.parent_id = dtd.file_id
        )
        SELECT file_id FROM directories_to_delete
    );

    INSERT INTO permission_app.events(user_id, directory_id, event)
    VALUES (requester_id, new_file_id, 'DELETE');

    RETURN 'Success';
END;
$$;

-- Предоставляем права на выполнение функции
GRANT EXECUTE ON FUNCTION permission_app.DeleteFileByNameAndUser(TEXT, TEXT) TO app_user;