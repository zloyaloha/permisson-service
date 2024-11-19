-- Permissions table
-- depends: 20241118_01_1E8ID-get-user-rights

CREATE TABLE IF NOT EXISTS permission_app.permissions (
    permission_id SERIAL PRIMARY KEY,
    node_id INT REFERENCES permission_app.nodes(file_id) ON DELETE CASCADE, -- Узел, к которому применяются права
    user_id INT REFERENCES permission_app.users(user_id) ON DELETE CASCADE, -- Пользователь (NULL, если права для группы)
    group_id INT REFERENCES permission_app.groups(group_id) ON DELETE CASCADE, -- Группа (NULL, если права для пользователя)
    can_read BOOLEAN DEFAULT FALSE,
    can_write BOOLEAN DEFAULT FALSE,
    can_execute BOOLEAN DEFAULT FALSE,
    UNIQUE(node_id, user_id, group_id) -- Гарантирует уникальность комбинаций
);

GRANT SELECT, INSERT, DELETE, UPDATE ON permission_app.permissions TO app_user;
GRANT USAGE, SELECT ON SEQUENCE permission_app.permissions_permission_id_seq TO app_user;
