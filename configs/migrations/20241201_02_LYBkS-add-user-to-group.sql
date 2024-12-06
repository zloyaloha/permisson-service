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
    SELECT permission_app.UserID(p_login)
    INTO v_user_id;
    
    IF v_user_id IS NULL THEN
        RETURN 'User Not Found';
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

    RETURN 'Success';
END;
$$ LANGUAGE plpgsql SECURITY DEFINER;

GRANT EXECUTE ON FUNCTION permission_app.AddUserToGroup(TEXT, TEXT) TO app_user;

CREATE OR REPLACE FUNCTION permission_app.AddUserToGroupEvent()
RETURNS TRIGGER AS $$
DECLARE
    u_login TEXT;
    g_group_name TEXT;
BEGIN

    SELECT login INTO u_login
    FROM permission_app.users
    WHERE user_id = NEW.user_id;

    SELECT name INTO g_group_name
    FROM permission_app.groups
    WHERE group_id = NEW.group_id;

    INSERT INTO permission_app.user_events(user_id, event, description)
    VALUES (NEW.user_id, 'ADD_USER_TO_GROUP', u_login || ' ' || g_group_name);

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

DROP TRIGGER IF EXISTS AddUserToGroupEventTrigger ON permission_app.user_to_group;
CREATE TRIGGER AddUserToGroupEventTrigger
AFTER INSERT ON permission_app.user_to_group
FOR EACH ROW
EXECUTE FUNCTION permission_app.AddUserToGroupEvent();
