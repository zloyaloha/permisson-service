-- Session create
-- depends: 20241108_01_kHThF-session-table

CREATE OR REPLACE FUNCTION permission_app.AddSession(
    user_id INT
)
RETURNS TEXT
LANGUAGE plpgsql
AS
$$
DECLARE
    new_session_token TEXT;
BEGIN
    new_session_token := encode(gen_random_bytes(32), 'hex');

    INSERT INTO permission_app.sessions (
        user_id,
        session_token
    )
    VALUES (
        user_id,
        new_session_token
    );

    RETURN new_session_token;
END;
$$;

GRANT EXECUTE ON FUNCTION permission_app.AddSession(INT) TO app_user;