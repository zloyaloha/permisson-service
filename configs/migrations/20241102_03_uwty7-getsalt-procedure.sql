-- GetSalt procedure
-- depends: 20241102_02_ZCVSm-create-schemas-and-db-users

CREATE OR REPLACE FUNCTION permission_app.GetSaltAndPassword(username TEXT)
    RETURNS TABLE(user_password VARCHAR(100), user_salt VARCHAR(100))
    LANGUAGE plpgsql
AS
$$
DECLARE
    userSalt TEXT := '';
begin
    RETURN QUERY
    SELECT password, salt
    FROM permission_app.users
    WHERE login = username;
end;
$$ SECURITY DEFINER;

GRANT EXECUTE ON FUNCTION permission_app.GetSaltAndPassword(text) TO app_user;
