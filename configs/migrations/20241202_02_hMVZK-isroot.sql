-- IsRoot
-- depends: 20241202_01_nu78A-changerigts

CREATE OR REPLACE FUNCTION permission_app.IsRoot(
    userName TEXT
)
RETURNS BOOLEAN AS
$$
DECLARE
    is_root BOOL;
BEGIN

    SELECT root INTO is_root
    FROM permission_app.users
    WHERE login = userName;

    RETURN is_root;
END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.IsRoot(TEXT) TO app_user;