-- Session table
-- depends: 20241105_02_IvWLR-create-user-procedure

CREATE TABLE IF NOT EXISTs permission_app.sessions (
    session_id SERIAL PRIMARY KEY,
    user_id INT REFERENCES permission_app.users(user_id) ON DELETE CASCADE,
    session_token TEXT UNIQUE NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_activity TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    exit_at TIMESTAMP DEFAULT NULL
);

GRANT SELECT, INSERT, DELETE, UPDATE ON permission_app.sessions TO app_user;
GRANT USAGE, SELECT, UPDATE ON SEQUENCE permission_app.sessions_session_id_seq TO app_user;
