-- Create group
-- depends: 20241201_02_LYBkS-add-user-to-group

CREATE OR REPLACE FUNCTION permission_app.CreateGroup(group_name TEXT, creator_username TEXT)
RETURNS VARCHAR AS
$$
DECLARE
    existing_group_id INT;
    creator_user_id INT;
BEGIN
    -- Получаем user_id пользователя по его имени
    SELECT user_id INTO creator_user_id
    FROM permission_app.users
    WHERE login = creator_username AND root = TRUE;

    -- Если пользователь не найден, возвращаем 'User Not Found'
    IF creator_user_id IS NULL THEN
        RETURN 'User Not Found';
    END IF;

    -- Проверяем, существует ли уже группа с таким именем
    SELECT group_id INTO existing_group_id
    FROM permission_app.groups
    WHERE name = group_name;

    -- Если группа существует, возвращаем 'Already Exists'
    IF existing_group_id IS NOT NULL THEN
        RETURN 'Already Exists';
    END IF;

    -- Вставляем новую группу
    INSERT INTO permission_app.groups (name)
    VALUES (group_name);

    -- Добавляем запись в таблицу user_events
    INSERT INTO permission_app.user_events (user_id, event, description)
    VALUES (creator_user_id, 'CREATE_GROUP', group_name);

    -- Возвращаем 'Success', если группа была успешно создана
    RETURN 'Success';
END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.CreateGroup(TEXT, TEXT) TO app_user;