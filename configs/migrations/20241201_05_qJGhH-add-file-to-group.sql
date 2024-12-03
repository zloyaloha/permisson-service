-- Add file to group
-- depends: 20241201_04_eCFXD-delete-group

CREATE OR REPLACE FUNCTION permission_app.AddFileToGroup(
    file_name VARCHAR,
    group_name VARCHAR,
    username VARCHAR
)
RETURNS VARCHAR AS
$$
DECLARE
    user_id_selected INT;
    group_id_selected INT;
    file_id_selected INT;
BEGIN
    -- Получаем ID пользователя
    SELECT permission_app.UserID(username)
    INTO user_id_selected;

    -- Если пользователь не найден
    IF user_id_selected IS NULL THEN
        RETURN 'User Not Found';
    END IF;

    -- Получаем ID группы
    SELECT group_id INTO group_id_selected
    FROM permission_app.groups
    WHERE name = group_name;

    -- Если группа не найдена
    IF group_id_selected IS NULL THEN
        RETURN 'Group Not Found';
    END IF;

    -- Получаем ID файла
    SELECT file_id INTO file_id_selected
    FROM permission_app.nodes
    WHERE name = file_name;

    -- Если файл не найден
    IF file_id_selected IS NULL THEN
        RETURN 'File Not Found';
    END IF;

    -- Проверяем, существует ли уже запись в таблице permissions
    IF EXISTS (
        SELECT 1
        FROM permission_app.permissions_group
        WHERE node_id = file_id_selected AND group_id = group_id_selected
    ) THEN
        RETURN 'File Already Added';
    END IF;

    UPDATE permission_app.permissions_group
    SET group_id = group_id_selected
    WHERE node_id = file_id_selected;

    INSERT INTO permission_app.user_events (user_id, event, description)
    VALUES (user_id_selected, 'ADD_FILE_TO_GROUP', 'File "' || file_name || '" added to group "' || group_name || '"');

    -- Возвращаем успешное сообщение
    RETURN 'Success';
END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.AddFileToGroup(VARCHAR, VARCHAR, VARCHAR) TO app_user;
