-- File table
-- depends: 20241108_04_CHDdk-off-session

CREATE TYPE event_type AS ENUM ('READ', 'WRITE', 'DELETE', 'CHANGE_PERMISSION');

CREATE TABLE IF NOT EXISTS permission_app.events (
    event_id SERIAL PRIMARY KEY,
    user_id INT REFERENCES permission_app.users(user_id) ON DELETE CASCADE,
    file_id INT,
    event event_type,
    event_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS permission_app.files (
    file_id SERIAL PRIMARY KEY,
    path VARCHAR(255),
    event_id INT
);

GRANT SELECT, INSERT, DELETE, UPDATE ON permission_app.files TO app_user;
GRANT SELECT, INSERT, DELETE, UPDATE ON permission_app.events TO app_user;

ALTER TABLE permission_app.files
ADD CONSTRAINT fk_event_file FOREIGN KEY (event_id) REFERENCES permission_app.events(event_id);

ALTER TABLE permission_app.events
ADD CONSTRAINT fk_event_file FOREIGN KEY (file_id) REFERENCES permission_app.files(file_id) ON DELETE CASCADE;