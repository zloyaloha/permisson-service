-- Get users list
-- depends: 20241130_02_sil2i-delete-active-sessions

CREATE OR REPLACE FUNCTION permission_app.GetUsersWithStatus()
RETURNS TABLE (
    login VARCHAR(50),
    is_admin BOOLEAN,
    is_active BOOLEAN
) 
LANGUAGE plpgsql
AS $$
BEGIN
    RETURN QUERY
    SELECT 
        u.login,
        u.root AS is_admin,
        CASE 
            WHEN EXISTS (
                SELECT 1 
                FROM permission_app.sessions s 
                WHERE s.user_id = u.user_id AND s.exit_at IS NULL
            ) THEN TRUE
            ELSE FALSE
        END AS is_active
    FROM 
        permission_app.users u;
END;
$$;

GRANT EXECUTE ON FUNCTION permission_app.GetUsersWithStatus() TO app_user;

