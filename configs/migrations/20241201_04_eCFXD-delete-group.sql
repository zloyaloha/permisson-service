-- Delete group
-- depends: 20241201_03_xzTIa-create-group

CREATE OR REPLACE FUNCTION permission_app.DeleteGroup(group_name VARCHAR, admin_username VARCHAR)
RETURNS VARCHAR AS
$$
DECLARE
    existing_group_id INT;
    admin_user_id INT;
BEGIN
    SELECT permission_app.UserID(admin_username)
    INTO admin_user_id;

    IF admin_user_id IS NULL THEN
        RETURN 'User Not Found';
    END IF;

    SELECT group_id INTO existing_group_id
    FROM permission_app.groups
    WHERE name = group_name;

    -- Если группа не найдена, возвращаем 'Group Not Found'
    IF existing_group_id IS NULL THEN
        RETURN 'Group Not Found';
    END IF;

    -- Удаляем пользователей из этой группы
    DELETE FROM permission_app.user_to_group
    WHERE group_id = existing_group_id;

    -- Удаляем саму группу
    DELETE FROM permission_app.groups
    WHERE group_id = existing_group_id;

    -- Добавляем запись в таблицу user_events о том, что группа была удалена
    INSERT INTO permission_app.user_events (user_id, event, description)
    VALUES (admin_user_id, 'DELETE_GROUP', group_name);

    -- Возвращаем 'Success', если группа была успешно удалена
    RETURN 'Success';
END;
$$ LANGUAGE plpgsql SECURITY DEFINER;

GRANT EXECUTE ON FUNCTION permission_app.DeleteGroup(VARCHAR, VARCHAR) TO app_user;