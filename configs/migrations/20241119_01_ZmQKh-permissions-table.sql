-- Permissions table
-- depends: 20241118_01_1E8ID-get-file-id

-- Таблица для прав доступа пользователей
CREATE TABLE IF NOT EXISTS permission_app.permissions_user (
    permission_id SERIAL PRIMARY KEY,
    node_id INT REFERENCES permission_app.nodes(file_id) ON DELETE CASCADE, -- Узел
    user_id INT REFERENCES permission_app.users(user_id) ON DELETE CASCADE, -- Пользователь
    can_read BOOLEAN DEFAULT TRUE,
    can_write BOOLEAN DEFAULT TRUE,
    can_execute BOOLEAN DEFAULT TRUE,
    UNIQUE(node_id)
);

-- Таблица для прав доступа групп
CREATE TABLE IF NOT EXISTS permission_app.permissions_group (
    permission_id SERIAL PRIMARY KEY,
    node_id INT REFERENCES permission_app.nodes(file_id) ON DELETE CASCADE, -- Узел
    group_id INT REFERENCES permission_app.groups(group_id) ON DELETE CASCADE, -- Группа
    can_read BOOLEAN DEFAULT TRUE,
    can_write BOOLEAN DEFAULT FALSE,
    can_execute BOOLEAN DEFAULT TRUE,
    UNIQUE(node_id)
);

CREATE TABLE IF NOT EXISTS permission_app.permissions_all (
    permission_id SERIAL PRIMARY KEY,
    node_id INT REFERENCES permission_app.nodes(file_id) ON DELETE CASCADE, -- Узел
    can_read BOOLEAN DEFAULT TRUE,
    can_write BOOLEAN DEFAULT FALSE,
    can_execute BOOLEAN DEFAULT TRUE,
    UNIQUE(node_id)
);

INSERT INTO permission_app.permissions_user(node_id)
VALUES (1);

INSERT INTO permission_app.permissions_group(node_id)
VALUES (1);

INSERT INTO permission_app.permissions_all(node_id)
VALUES (1);

GRANT SELECT, INSERT, DELETE, UPDATE ON permission_app.permissions_user TO app_user;
GRANT SELECT, INSERT, DELETE, UPDATE ON permission_app.permissions_group TO app_user;
GRANT SELECT, INSERT, DELETE, UPDATE ON permission_app.permissions_all TO app_user;

GRANT USAGE, SELECT ON SEQUENCE permission_app.permissions_user_permission_id_seq TO app_user;
GRANT USAGE, SELECT ON SEQUENCE permission_app.permissions_group_permission_id_seq TO app_user;
GRANT USAGE, SELECT ON SEQUENCE permission_app.permissions_all_permission_id_seq TO app_user;

CREATE OR REPLACE FUNCTION permission_app.GetNodeGroup(node_id_in INT)
RETURNS TEXT AS $$
DECLARE
    group_name TEXT;
BEGIN
    SELECT g.name
    INTO group_name
    FROM permission_app.permissions_group pg
    JOIN permission_app.groups g ON pg.group_id = g.group_id
    WHERE pg.node_id = node_id_in
    LIMIT 1;

    RETURN group_name;
END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.GetNodeGroup(INT) TO app_user;


CREATE OR REPLACE FUNCTION permission_app.GetNodeUser(node_id_in INT)
RETURNS TEXT AS $$
DECLARE
    user_name TEXT;
BEGIN
    SELECT u.login
    INTO user_name
    FROM permission_app.permissions_user pu
    JOIN permission_app.users u ON pu.user_id = u.user_id
    WHERE pu.node_id = node_id_in
    LIMIT 1;

    RETURN user_name;
END;
$$ LANGUAGE plpgsql;

GRANT EXECUTE ON FUNCTION permission_app.GetNodeGroup(INT) TO app_user;
