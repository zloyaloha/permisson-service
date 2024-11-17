-- File table
-- depends: 20241116_02_3MEzB-user-to-group

CREATE TYPE event_type AS ENUM ('READ', 'WRITE', 'DELETE', 'CHANGE_PERMISSION');

CREATE TABLE IF NOT EXISTS permission_app.events (
    event_id SERIAL PRIMARY KEY
    user_id REFERENCES permission_app.users(user_id)
    file_id REFERENCES permission_app.files(file_id)
    event event_type
    event_time DEFAULT CURRENT TIMESTAMP
);

CREATE TABLE IF NOT EXISTS permission_app.files (
    file_id SERIAL PRIMARY KEY
    path VARCHAR<255>
    event_id REFERENCES permission_app.events(event_id) ON DELETE CASCADE
);

GRANT SELECT, INSERT, DELETE, UPDATE ON permission_app.files TO app_user;
GRANT SELECT, INSERT, DELETE, UPDATE ON permission_app.events TO app_user;