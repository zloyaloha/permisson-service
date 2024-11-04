-- GetSalt procedure
-- depends: 20241102_02_ZCVSm-create-schemas-and-db-users

CREATE OR REPLACE FUNCTION permission_app.GetSalt(username TEXT)
    RETURNS TEXT
    LANGUAGE plpgsql
AS
$$
DECLARE
    userSalt TEXT := '';
begin
    SELECT salt
    INTO userSalt
    FROM permission_app.users
    WHERE login = username;
    RETURN COALESCE(userSalt, '');
end;
$$;

GRANT EXECUTE ON FUNCTION permission_app.GetSalt(text) TO app_user;
