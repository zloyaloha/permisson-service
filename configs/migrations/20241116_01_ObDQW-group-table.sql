-- File object
-- depends: 20241108_04_CHDdk-off-session

CREATE TABLE IF NOT EXISTS permission_app.groups (
    group_id SERIAL PRIMARY KEY 
);

GRANT SELECT, INSERT, DELETE, UPDATE ON permission_app.groups TO app_user;