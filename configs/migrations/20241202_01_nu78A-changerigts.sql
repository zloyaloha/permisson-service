-- ChangeRigts
-- depends: 20241201_05_qJGhH-add-file-to-group

CREATE OR REPLACE FUNCTION permission_app.IsUserOwnerOrRoot(
    userName TEXT,
    fileName TEXT
)
RETURNS BOOLEAN AS
$$
DECLARE
    file_owner_id INT;    -- ID владельца файла
    username_user_id INT;          -- ID пользователя
    is_root BOOL;
BEGIN
    -- Получаем ID владельца файла
    SELECT user_id INTO file_owner_id
    FROM permission_app.nodes n LEFT JOIN permission_app.permissions_user pu ON (n.file_id = pu.node_id)
    WHERE n.name = fileName;

    -- Получаем ID пользователя
    SELECT user_id, is_root INTO username_user_id
    FROM permission_app.users
    WHERE login = userName;

    -- Проверяем, является ли пользователь владельцем файла или root
    RETURN (file_owner_id = username_user_id OR is_root);
END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.IsUserOwnerOrRoot(TEXT, TEXT) TO app_user;


CREATE OR REPLACE FUNCTION permission_app.UpdateUserPermissionsByMask(
    fileName TEXT,       -- Идентификатор узла
    p_permissions_mask INT -- Битовая маска прав (например, 7 для rwx)
)
RETURNS VARCHAR AS 
$$
DECLARE
    node_id_selected INT;
BEGIN

    SELECT file_id INTO node_id_selected
    FROM permission_app.nodes
    WHERE name = fileName;

    UPDATE permission_app.permissions_user
    SET
        can_read = (p_permissions_mask & 1) > 0,  -- Проверка бита 0
        can_write = (p_permissions_mask & 2) > 0, -- Проверка бита 1
        can_execute = (p_permissions_mask & 4) > 0 -- Проверка бита 2
    WHERE node_id = node_id_selected;

    IF NOT FOUND THEN
        RETURN 'File Not Found';
    END IF;

    RETURN 't';
    
END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.UpdateUserPermissionsByMask(TEXT, INT) TO app_user;

CREATE OR REPLACE FUNCTION permission_app.UpdateGroupPermissionsByMask(
    fileName TEXT,       -- Идентификатор узла
    p_permissions_mask INT -- Битовая маска прав (например, 7 для rwx)
)
RETURNS VARCHAR AS 
$$
DECLARE
    node_id_selected INT;
BEGIN

    SELECT file_id INTO node_id_selected
    FROM permission_app.nodes
    WHERE name = fileName;

    UPDATE permission_app.permissions_group
    SET
        can_read = (p_permissions_mask & 1) > 0,  -- Проверка бита 0
        can_write = (p_permissions_mask & 2) > 0, -- Проверка бита 1
        can_execute = (p_permissions_mask & 4) > 0 -- Проверка бита 2
    WHERE node_id = node_id_selected;

    IF NOT FOUND THEN
        RETURN 'Group Not Found';
    END IF;

    RETURN 't';
    
END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.UpdateGroupPermissionsByMask(TEXT, INT) TO app_user;

CREATE OR REPLACE FUNCTION permission_app.UpdateAllPermissionsByMask(
    fileName TEXT,       -- Идентификатор узла
    p_permissions_mask INT -- Битовая маска прав (например, 7 для rwx)
)
RETURNS VARCHAR AS 
$$
DECLARE
    node_id_selected INT;
BEGIN

    SELECT file_id INTO node_id_selected
    FROM permission_app.nodes
    WHERE name = fileName;

    UPDATE permission_app.permissions_all
    SET
        can_read = (p_permissions_mask & 1) > 0,  -- Проверка бита 0
        can_write = (p_permissions_mask & 2) > 0, -- Проверка бита 1
        can_execute = (p_permissions_mask & 4) > 0 -- Проверка бита 2
    WHERE node_id = node_id_selected;

    IF NOT FOUND THEN
        RETURN 'All Not Found';
    END IF;

    RETURN 't';
    
END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.UpdateAllPermissionsByMask(TEXT, INT) TO app_user;

CREATE OR REPLACE FUNCTION permission_app.AddEvent(
    fileName TEXT,
    userName TEXT,
    p_permissions_mask TEXT
)
RETURNS VOID AS 
$$
DECLARE
    node_id_selected INT;
    user_id_selected INT;
BEGIN
    
    SELECT file_id INTO node_id_selected
    FROM permission_app.nodes
    WHERE name = fileName;

    SELECT user_id INTO user_id_selected
    FROM permission_app.users
    WHERE login = userName;

    INSERT INTO permission_app.events(user_id, file_id, event, description)
    VALUES (user_id_selected, node_id_selected, 'CHANGE_PERMISSION', p_permissions_mask);

END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.AddEvent(TEXT, TEXT, TEXT) TO app_user;