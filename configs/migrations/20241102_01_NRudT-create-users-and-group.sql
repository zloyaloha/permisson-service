-- Create users and group
-- depends: 

CREATE TABLE IF NOT EXISTs users (
    user_id SERIAL PRIMARY KEY,
    login VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(100) NOT NULL UNIQUE,
    salt VARCHAR(50) NOT NULL,
    time_registration TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    root BOOL DEFAULT FALSE
);

CREATE TABLE IF NOT EXISTS groups (
    group_id SERIAL PRIMARY KEY
);

CREATE TABLE IF NOT EXISTS user_to_group (
    user_id INTEGER REFERENCES users(user_id),
    group_id INTEGER REFERENCES groups(group_id)
);