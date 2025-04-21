-- Off session
-- depends: 20241108_03_A6eoh-get-token

DROP PROCEDURE IF EXISTS permission_app.UpdateExitTime(INT);
CREATE OR REPLACE PROCEDURE permission_app.UpdateExitTime(
    id INT
)
LANGUAGE plpgsql
AS
$$
BEGIN
    UPDATE permission_app.sessions
    SET exit_at = CURRENT_TIMESTAMP
    WHERE user_id = id;
END;
$$;
