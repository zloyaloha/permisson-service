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

    -- Получаем ID директории и её владельца и группу
    SELECT n.file_id, n.owner_id, n.group_id INTO directory_id, owner_id, owner_group_id
    FROM permission_app.nodes n
    WHERE n.name = directory_name;

    IF directory_id IS NULL THEN
        RETURN 'Directory not found';
    END IF;

    -- Проверяем, принадлежит ли пользователь группе-владельцу
    SELECT EXISTS(
        SELECT 1
        FROM permission_app.user_to_group
        WHERE user_id = requester_id AND group_id = owner_group_id
    ) INTO belongs_to_group;

    -- Проверка прав пользователя
    IF NOT (requester_id = owner_id OR is_root OR belongs_to_group) THEN
        RETURN 'Access denied';
    END IF;

    -- Логируем событие удаления перед удалением данных
    INSERT INTO permission_app.events (user_id, file_id, event)
    VALUES (requester_id, directory_id, 'DELETE');

    -- Удаление директории и её содержимого рекурсивно
    DELETE FROM permission_app.nodes
    WHERE file_id IN (
        WITH RECURSIVE nodes_to_delete AS (
            SELECT file_id
            FROM permission_app.nodes
            WHERE file_id = directory_id
            UNION ALL
            SELECT n.file_id
            FROM permission_app.nodes n
            INNER JOIN nodes_to_delete dtd ON n.parent_id = dtd.file_id
        )
        SELECT file_id FROM nodes_to_delete
    );

    RETURN 'Directory and contents successfully deleted';
END;
$$;

-- Предоставляем права на выполнение функции
GRANT EXECUTE ON FUNCTION permission_app.DeleteFileByNameAndUser(TEXT, TEXT) TO app_user;
