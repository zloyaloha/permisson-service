-- Session create
-- depends: 20241108_01_kHThF-session-table

CREATE OR REPLACE FUNCTION permission_app.AddSession(
    id INT
)
RETURNS TEXT
LANGUAGE plpgsql
AS
$$
DECLARE
    new_session_token TEXT;
BEGIN

    IF EXISTS (
        SELECT 1
        FROM permission_app.sessions
        WHERE user_id = id AND exit_at IS NULL
    ) THEN
        RETURN 'Session exists';
    END IF;

    new_session_token := encode(gen_random_bytes(32), 'hex');

    INSERT INTO permission_app.sessions (
        user_id,
        session_token
    )
    VALUES (
        id,
        new_session_token
    );

    RETURN new_session_token;
END;
$$;

GRANT EXECUTE ON FUNCTION permission_app.AddSession(INT) TO app_user;