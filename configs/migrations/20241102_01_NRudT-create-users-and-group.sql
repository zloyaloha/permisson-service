-- Create users and group
-- depends: 
DROP TYPE IF EXISTS user_event_type CASCADE;
CREATE TYPE user_event_type AS ENUM ('CREATE_GROUP', 'DELETE_GROUP', 'ADD_USER_TO_GROUP', 'ADD_FILE_TO_GROUP');

CREATE TABLE IF NOT EXISTs users (
    user_id SERIAL PRIMARY KEY,
    login VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(100) NOT NULL UNIQUE,
    salt VARCHAR(50) NOT NULL,
    time_registration TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    root BOOL DEFAULT FALSE
);

CREATE TABLE IF NOT EXISTS groups (
    group_id SERIAL PRIMARY KEY,
    name VARCHAR(255) UNIQUE NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS user_to_group (
    user_id INTEGER REFERENCES users(user_id),
    group_id INTEGER REFERENCES groups(group_id),
    UNIQUE(group_id, user_id)
);

CREATE TABLE IF NOT EXISTS user_events (
    user_id INTEGER REFERENCES users(user_id),
    event user_event_type,
    description VARCHAR(255) DEFAULT '',
    time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);


