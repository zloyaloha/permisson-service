-- Get token
-- depends: 20241108_02_CvI6L-session-create

CREATE OR REPLACE FUNCTION permission_app.GetToken(
    username TEXT
)
RETURNS TEXT
LANGUAGE plpgsql
AS
$$
DECLARE
    user_token TEXT;
BEGIN
    SELECT session_token
    INTO user_token
    FROM permission_app.sessions
    WHERE 
        user_id = (SELECT user_id FROM permission_app.users WHERE login = username) 
        AND exit_at IS NULL;

    RETURN user_token;
END;
$$;

GRANT EXECUTE ON FUNCTION permission_app.GetToken(TEXT) TO app_user;