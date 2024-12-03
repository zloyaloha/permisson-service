-- File operations
-- depends: 20241202_02_hMVZK-isroot

CREATE OR REPLACE FUNCTION permission_app.CheckUserRightsToWrite(
    userName TEXT,
    fileName TEXT
)
RETURNS BOOLEAN AS
$$
DECLARE
    userName_user_id INT;
    fileName_file_id INT;
    is_group_in_user_groups BOOL;
    file_group VARCHAR(50);
    file_user VARCHAR(50);
    can_group_write BOOL;
    can_all_write BOOL;
    can_user_write BOOL;
    is_user_in_group BOOL;
    is_user_owner BOOL;
BEGIN
    -- Получаем ID пользователя
    SELECT permission_app.UserID(userName)
    INTO userName_user_id;

    -- Получаем ID файла
    SELECT permission_app.FileId(fileName)
    INTO fileName_file_id;

    -- Проверка прав "все" на запись
    SELECT can_write INTO can_all_write
    FROM permission_app.permissions_all
    WHERE node_id = fileName_file_id;

    IF can_all_write THEN
        RETURN TRUE;
    END IF;

    -- Проверка прав группы на запись
    SELECT can_write INTO can_group_write
    FROM permission_app.permissions_group
    WHERE node_id = fileName_file_id;

    SELECT permission_app.GetNodeGroup(fileName_file_id)
    INTO file_group;

    SELECT permission_app.IsUserInGroup(userName, file_group)
    INTO is_user_in_group;

    IF can_group_write AND is_user_in_group THEN
        RETURN TRUE;
    END IF;

    -- Проверка прав пользователя на запись
    SELECT can_write INTO can_user_write
    FROM permission_app.permissions_user
    WHERE node_id = fileName_file_id;

    SELECT permission_app.GetNodeUser(fileName_file_id)
    INTO file_user;

    -- Проверка, является ли пользователь владельцем файла
    IF file_user = userName THEN
        is_user_owner := TRUE;
    ELSE
        is_user_owner := FALSE;
    END IF;

    IF can_user_write AND is_user_owner THEN
        RETURN TRUE;
    END IF;

    -- Если ни одно из условий не выполнено, доступ запрещен
    RETURN FALSE;
END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.CheckUserRightsToWrite(TEXT, TEXT) TO app_user;



CREATE OR REPLACE FUNCTION permission_app.CheckUserRightsToRead(
    userName TEXT,
    fileName TEXT
)
RETURNS BOOLEAN AS
$$
DECLARE
    userName_user_id INT;
    fileName_file_id INT;
    is_group_in_user_groups BOOL;
    file_group VARCHAR(50);
    file_user VARCHAR(50);
    can_group_read BOOL;
    can_all_read BOOL;
    can_user_read BOOL;
    is_user_in_group BOOL;
    is_user_owner BOOL;
BEGIN
    -- Получаем ID пользователя
    SELECT permission_app.UserID(userName)
    INTO userName_user_id;

    -- Получаем ID файла
    SELECT permission_app.FileId(fileName)
    INTO fileName_file_id;

    -- Проверка прав "все" на запись
    SELECT can_read INTO can_all_read
    FROM permission_app.permissions_all
    WHERE node_id = fileName_file_id;

    IF can_all_read THEN
        RETURN TRUE;
    END IF;

    -- Проверка прав группы на запись
    SELECT can_read INTO can_group_read
    FROM permission_app.permissions_group
    WHERE node_id = fileName_file_id;

    SELECT permission_app.GetNodeGroup(fileName_file_id)
    INTO file_group;

    SELECT permission_app.IsUserInGroup(userName, file_group)
    INTO is_user_in_group;

    IF can_group_read AND is_user_in_group THEN
        RETURN TRUE;
    END IF;

    -- Проверка прав пользователя на запись
    SELECT can_read INTO can_user_read
    FROM permission_app.permissions_user
    WHERE node_id = fileName_file_id;

    SELECT permission_app.GetNodeUser(fileName_file_id)
    INTO file_user;

    -- Проверка, является ли пользователь владельцем файла
    IF file_user = userName THEN
        is_user_owner := TRUE;
    ELSE
        is_user_owner := FALSE;
    END IF;

    IF can_user_read AND is_user_owner THEN
        RETURN TRUE;
    END IF;

    -- Если ни одно из условий не выполнено, доступ запрещен
    RETURN FALSE;
END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.CheckUserRightsToRead(TEXT, TEXT) TO app_user;


CREATE OR REPLACE FUNCTION permission_app.CheckUserRightsToExec(
    userName TEXT,
    fileName TEXT
)
RETURNS BOOLEAN AS
$$
DECLARE
    userName_user_id INT;
    fileName_file_id INT;
    is_group_in_user_groups BOOL;
    file_group VARCHAR(50);
    file_user VARCHAR(50);
    can_group_exec BOOL;
    can_all_exec BOOL;
    can_user_exec BOOL;
    is_user_in_group BOOL;
    is_user_owner BOOL;
BEGIN
    -- Получаем ID пользователя
    SELECT permission_app.UserID(userName)
    INTO userName_user_id;

    -- Получаем ID файла
    SELECT permission_app.FileId(fileName)
    INTO fileName_file_id;

    -- Проверка прав "все" на запись
    SELECT can_exec INTO can_all_exec
    FROM permission_app.permissions_all
    WHERE node_id = fileName_file_id;

    IF can_all_exec THEN
        RETURN TRUE;
    END IF;

    -- Проверка прав группы на запись
    SELECT can_exec INTO can_group_exec
    FROM permission_app.permissions_group
    WHERE node_id = fileName_file_id;

    SELECT permission_app.GetNodeGroup(fileName_file_id)
    INTO file_group;

    SELECT permission_app.IsUserInGroup(userName, file_group)
    INTO is_user_in_group;

    IF can_group_exec AND is_user_in_group THEN
        RETURN TRUE;
    END IF;

    -- Проверка прав пользователя на запись
    SELECT can_exec INTO can_user_exec
    FROM permission_app.permissions_user
    WHERE node_id = fileName_file_id;

    SELECT permission_app.GetNodeUser(fileName_file_id)
    INTO file_user;

    -- Проверка, является ли пользователь владельцем файла
    IF file_user = userName THEN
        is_user_owner := TRUE;
    ELSE
        is_user_owner := FALSE;
    END IF;

    IF can_user_exec AND is_user_owner THEN
        RETURN TRUE;
    END IF;

    -- Если ни одно из условий не выполнено, доступ запрещен
    RETURN FALSE;
END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.CheckUserRightsToExec(TEXT, TEXT) TO app_user;
