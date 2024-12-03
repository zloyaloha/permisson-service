-- Add events
-- depends: 20241203_01_EN33C-file-operations

CREATE OR REPLACE PROCEDURE permission_app.AddWriteEvent(
    userName TEXT,
    fileName TEXT
)
LANGUAGE plpgsql
AS $$
DECLARE
    userName_user_id INT;
    fileName_file_id INT;
BEGIN
    -- Получаем ID пользователя
    SELECT permission_app.UserID(userName)
    INTO userName_user_id;

    -- Получаем ID файла
    SELECT permission_app.FileId(fileName)
    INTO fileName_file_id;

    INSERT INTO permission_app.events(user_id, file_id, event)
    VALUES (userName_user_id, fileName_file_id, 'WRITE');

END;
$$;


CREATE OR REPLACE PROCEDURE permission_app.AddReadEvent(
    userName TEXT,
    fileName TEXT
)
LANGUAGE plpgsql
AS $$
DECLARE
    userName_user_id INT;
    fileName_file_id INT;
BEGIN
    -- Получаем ID пользователя
    SELECT permission_app.UserID(userName)
    INTO userName_user_id;

    -- Получаем ID файла
    SELECT permission_app.FileId(fileName)
    INTO fileName_file_id;

    INSERT INTO permission_app.events(user_id, file_id, event)
    VALUES (userName_user_id, fileName_file_id, 'READ');

END;
$$;


CREATE OR REPLACE PROCEDURE permission_app.AddExecEvent(
    userName TEXT,
    fileName TEXT
)
LANGUAGE plpgsql
AS $$
DECLARE
    userName_user_id INT;
    fileName_file_id INT;
BEGIN
    -- Получаем ID пользователя
    SELECT permission_app.UserID(userName)
    INTO userName_user_id;

    -- Получаем ID файла
    SELECT permission_app.FileId(fileName)
    INTO fileName_file_id;

    INSERT INTO permission_app.events(user_id, file_id, event)
    VALUES (userName_user_id, fileName_file_id, 'EXECUTE');

END;
$$;

GRANT EXECUTE ON PROCEDURE permission_app.AddWriteEvent(TEXT, TEXT) TO app_user;
GRANT EXECUTE ON PROCEDURE permission_app.AddReadEvent(TEXT, TEXT) TO app_user;
GRANT EXECUTE ON PROCEDURE permission_app.AddExecEvent(TEXT, TEXT) TO app_user;