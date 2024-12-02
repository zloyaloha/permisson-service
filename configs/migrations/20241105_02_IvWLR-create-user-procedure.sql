-- Create user procedure
-- depends: 20241105_01_c1xpp-find-user

CREATE OR REPLACE PROCEDURE permission_app.CreateUser(
    username TEXT,
    hashed_password TEXT,
    input_salt TEXT
)
LANGUAGE plpgsql
AS $$
DECLARE
    s_user_id INT;
    s_group_id INT;
BEGIN
    INSERT INTO permission_app.users (login, password, salt)
    VALUES (username, hashed_password, input_salt);

    INSERT INTO permission_app.groups (name)
    VALUES (username || '_group');

    SELECT user_id INTO s_user_id
    FROM permission_app.users
    WHERE login = username;

    SELECT group_id INTO s_group_id
    FROM permission_app.groups
    WHERE name = username || '_group';

    INSERT INTO permission_app.user_to_group(user_id, group_id)
    VALUES (s_user_id, s_group_id);
END;
$$;

GRANT EXECUTE ON PROCEDURE permission_app.CreateUser(text, text, text) TO app_user;