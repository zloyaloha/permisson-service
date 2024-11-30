-- Delete active sessions
-- depends: 20241130_01_lGp9f-delete-file

CREATE OR REPLACE FUNCTION permission_app.DeleteActiveSessions()
RETURNS VOID
LANGUAGE plpgsql
AS $$
BEGIN

    DELETE FROM permission_app.sessions
    WHERE exit_at IS NULL;

END;
$$;

GRANT EXECUTE ON FUNCTION permission_app.DeleteActiveSessions() TO app_user;
