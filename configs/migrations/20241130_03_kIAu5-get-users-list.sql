-- Get users list
-- depends: 20241130_02_sil2i-delete-active-sessions

CREATE OR REPLACE FUNCTION permission_app.GetUsersWithStatus()
RETURNS TABLE (
    id INT,
    login VARCHAR(50),
    is_admin BOOLEAN
)
LANGUAGE plpgsql
AS $$
BEGIN
    RETURN QUERY
    SELECT
        u.user_id AS id,
        u.login,
        u.root AS is_admin
    FROM
        permission_app.users u;
END;
$$ SECURITY DEFINER;

GRANT EXECUTE ON FUNCTION permission_app.GetUsersWithStatus() TO app_user;

