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
    SELECT permission_app.UserID(username)
    INTO requester_id;

    IF requester_id IS NULL THEN
        RETURN 'User not found';
    END IF;

    -- Получаем ID директории и её владельца и группу
    SELECT n.file_id INTO directory_id
    FROM permission_app.nodes n
    WHERE n.name = directory_name;

    IF directory_id IS NULL THEN
        RETURN 'Directory not found';
    END IF;

    -- Логируем событие удаления перед удалением данных

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

    INSERT INTO permission_app.events (user_id, event)
    VALUES (requester_id, 'DELETE');
    RETURN 'Success';
END;
$$;

-- Предоставляем права на выполнение функции
GRANT EXECUTE ON FUNCTION permission_app.DeleteFileByNameAndUser(TEXT, TEXT) TO app_user;
