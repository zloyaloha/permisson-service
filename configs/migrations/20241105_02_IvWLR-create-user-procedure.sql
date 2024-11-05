-- Create user procedure
-- depends: 20241105_01_c1xpp-find-user

CREATE OR REPLACE PROCEDURE permission_app.CreateUser(
    username TEXT,
    hashed_password TEXT,
    input_salt TEXT
)
LANGUAGE plpgsql
AS $$
BEGIN
    INSERT INTO permission_app.users (login, password, salt)
    VALUES (username, hashed_password, input_salt);
END;
$$;

GRANT EXECUTE ON PROCEDURE permission_app.CreateUser(text, text, text) TO app_user;