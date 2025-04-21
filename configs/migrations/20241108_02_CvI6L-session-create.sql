-- Session create
-- depends: 20241108_01_kHThF-session-table

CREATE OR REPLACE FUNCTION permission_app.AddSession(
    id INT
)
RETURNS BOOLEAN
LANGUAGE plpgsql
AS
$$
BEGIN

    IF EXISTS (
        SELECT 1
        FROM permission_app.sessions
        WHERE user_id = id AND exit_at IS NULL
    ) THEN
        RETURN FALSE;
    END IF;


    INSERT INTO permission_app.sessions (
        user_id
    )
    VALUES (
        id
    );

    RETURN TRUE;
END;
$$;

GRANT EXECUTE ON FUNCTION permission_app.AddSession(INT) TO app_user;