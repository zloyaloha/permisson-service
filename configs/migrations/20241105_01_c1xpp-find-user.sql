-- Find user
-- depends: 20241102_03_uwty7-getsalt-procedure

CREATE OR REPLACE FUNCTION permission_app.UserID(username TEXT)
    RETURNS INTEGER
    LANGUAGE plpgsql
AS
$$
DECLARE
    id INTEGER;
BEGIN
    SELECT user_id
    INTO id
    FROM permission_app.users
    WHERE login = username
    LIMIT 1;

    IF id IS NULL THEN
        RETURN 0;
    ELSE
        RETURN id;
    END IF;
END;
$$;

GRANT EXECUTE ON FUNCTION permission_app.UserID(text) TO app_user;

CREATE OR REPLACE FUNCTION permission_app.GetUserGroups(p_username TEXT)
RETURNS TABLE(group_name VARCHAR) AS $$
BEGIN
    RETURN QUERY
    SELECT g.name
    FROM permission_app.users u
    JOIN permission_app.user_to_group ug ON u.user_id = ug.user_id
    JOIN permission_app.groups g ON ug.group_id = g.group_id
    WHERE u.login = p_username;
END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.GetUserGroups(TEXT) TO app_user;

CREATE OR REPLACE FUNCTION permission_app.IsUserInGroup(p_username TEXT, p_groupname TEXT)
RETURNS BOOLEAN AS $$
BEGIN
    RETURN EXISTS (
        SELECT 1
        FROM permission_app.users u
        JOIN permission_app.user_to_group ug ON u.user_id = ug.user_id
        JOIN permission_app.groups g ON ug.group_id = g.group_id
        WHERE u.login = p_username AND g.name = p_groupname
    );
END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.IsUserInGroup(TEXT, TEXT) TO app_user;


CREATE OR REPLACE FUNCTION permission_app.GroupID(groupName TEXT)
    RETURNS INTEGER
    LANGUAGE plpgsql
AS
$$
DECLARE
    id INTEGER;
BEGIN
    SELECT group_id
    INTO id
    FROM permission_app.groups
    WHERE name = groupName
    LIMIT 1;

    IF id IS NULL THEN
        RETURN 0;
    ELSE
        RETURN id;
    END IF;
END;
$$;

GRANT EXECUTE ON FUNCTION permission_app.GroupID(text) TO app_user;