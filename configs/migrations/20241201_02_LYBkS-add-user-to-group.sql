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
    WHERE login = p_login AND root = TRUE;
    
    IF v_user_id IS NULL THEN
        RETURN 'No access';
    END IF;

    SELECT group_id INTO v_group_id
    FROM permission_app.groups
    WHERE name = p_group_name;
    
    IF v_group_id IS NULL THEN
        RETURN 'Group Not Found';
    END IF;

    INSERT INTO permission_app.user_to_group (user_id, group_id)
    VALUES (v_user_id, v_group_id)
    ON CONFLICT (user_id, group_id) DO NOTHING;

    INSERT INTO permission_app.user_events(user_id, event, description)
    VALUES (v_user_id, 'ADD_USER_TO_GROUP', p_login);

    RETURN 'Success';
END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.AddUserToGroup(TEXT, TEXT) TO app_user;