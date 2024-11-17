-- Get user rights
-- depends: 20241117_01_sAk0i-file-table

CREATE OR REPLACE FUNCTION permission_app.UserRights(username TEXT)
    RETURNS BOOL
    LANGUAGE plpgsql
AS
$$
DECLARE
    user_root BOOL;
BEGIN
    SELECT root
    INTO user_root
    FROM permission_app.users
    WHERE login = username
    LIMIT 1;
END;
$$;

GRANT EXECUTE ON FUNCTION permission_app.UserRights(text) TO app_user;