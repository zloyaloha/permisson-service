-- User to group
-- depends: 20241116_01_ObDQW-group-table

CREATE TABLE IF NOT EXISTS permission_app.user_to_group (
    group_id REFERENCES permission_app.groups(group_id) ON DELETE CASCADE,
    user_id REFERENCES permission_app.users(user_id) ON DELETE CASCADE
);

GRANT SELECT, INSERT, DELETE, UPDATE ON permission_app.user_to_group TO app_user;

