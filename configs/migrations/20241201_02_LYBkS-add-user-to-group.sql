-- Add user to group
-- depends: 20241201_01_vORjN-get-group-list

CREATE OR REPLACE FUNCTION permission_app.AddUserToGroup(
    p_group_name TEXT,
    p_login TEXT
)
RETURNS TEXT AS $$
DECLARE
    v_user_id INT;
    v_group_id INT;
BEGIN
    -- Находим user_id по login
    SELECT user_id INTO v_user_id
    FROM permission_app.users
    WHERE login = p_login;
    
    -- Если пользователь не найден, возвращаем ошибку
    IF v_user_id IS NULL THEN
        RETURN 'User Not Found';
    END IF;

    -- Находим group_id по group_name
    SELECT group_id INTO v_group_id
    FROM permission_app.groups
    WHERE name = p_group_name;
    
    -- Если группа не найдена, возвращаем ошибку
    IF v_group_id IS NULL THEN
        RETURN 'Group Not Found';
    END IF;

    -- Вставляем запись в таблицу user_to_group
    INSERT INTO permission_app.user_to_group (user_id, group_id)
    VALUES (v_user_id, v_group_id)
    ON CONFLICT (user_id, group_id) DO NOTHING;  -- Не вставлять, если такая связь уже существует

    -- Возвращаем успех, если операция прошла успешно
    RETURN 'Success';
END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.GetGroupList() TO app_user;