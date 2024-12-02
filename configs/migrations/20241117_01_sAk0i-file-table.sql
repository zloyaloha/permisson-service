-- File table
-- depends: 20241108_04_CHDdk-off-session

CREATE TYPE event_type AS ENUM ('READ', 'WRITE', 'DELETE', 'CHANGE_PERMISSION', 'CREATE_FILE', 'CREATE_DIR');
CREATE TYPE file_type AS ENUM ('FILE', 'DIR');

CREATE TABLE IF NOT EXISTS permission_app.nodes (
    file_id SERIAL PRIMARY KEY,
    parent_id INT REFERENCES permission_app.nodes(file_id) ON DELETE CASCADE,
    name VARCHAR(255) UNIQUE NOT NULL,
    type file_type NOT NULL,
    file_data BYTEA,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS permission_app.events (
    event_id SERIAL PRIMARY KEY,
    user_id INT REFERENCES permission_app.users(user_id) ON DELETE CASCADE,
    file_id INT REFERENCES permission_app.nodes(file_id) ON DELETE CASCADE,
    event event_type,
    event_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

INSERT INTO permission_app.nodes(parent_id, name, type)
VALUES (NULL, 'root', 'DIR');

GRANT SELECT, INSERT, DELETE, UPDATE ON permission_app.nodes TO app_user;
GRANT SELECT, INSERT, DELETE, UPDATE ON permission_app.events TO app_user;
GRANT USAGE, SELECT ON SEQUENCE permission_app.nodes_file_id_seq TO app_user;
GRANT USAGE, SELECT ON SEQUENCE permission_app.events_event_id_seq TO app_user;
