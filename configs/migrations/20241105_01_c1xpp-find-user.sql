-- Find user
-- depends: 20241102_03_uwty7-getsalt-procedure

CREATE OR REPLACE FUNCTION permission_app.UserExists(username TEXT)
    RETURNS BOOLEAN
    LANGUAGE plpgsql
AS
$$
DECLARE
    user_exists BOOLEAN;
BEGIN
    -- Проверка на существование пользователя с логином username
    SELECT EXISTS (
        SELECT 1
        FROM permission_app.users
        WHERE login = username
    ) INTO user_exists;

    RETURN user_exists;
END;
$$;

GRANT EXECUTE ON FUNCTION permission_app.UserExists(text) TO app_user;
