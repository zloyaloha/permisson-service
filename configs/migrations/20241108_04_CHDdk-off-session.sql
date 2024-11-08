-- Off session
-- depends: 20241108_03_A6eoh-get-token

CREATE OR REPLACE PROCEDURE permission_app.UpdateExitTime(
    token TEXT
)
LANGUAGE plpgsql
AS
$$
BEGIN
    UPDATE permission_app.sessions
    SET exit_at = CURRENT_TIMESTAMP
    WHERE session_token = token;

    RAISE NOTICE 'Exit time for session token % updated successfully', token;
END;
$$;

GRANT EXECUTE ON PROCEDURE permission_app.UpdateExitTime(TEXT) TO app_user;