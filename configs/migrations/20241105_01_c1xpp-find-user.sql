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