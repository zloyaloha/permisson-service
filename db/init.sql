-- CREATE TABLE users (
--     user_id SERIAL PRIMARY KEY,
--     login VARCHAR(50) NOT NULL,
--     password VARCHAR(100) NOT NULL,
--     session_id INT,
--     time_registration TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
--     root BOOL
-- );

-- CREATE TABLE groups (
--     group_id SERIAL PRIMARY KEY
-- );

-- CREATE TABLE user_to_group (
--     user_id INTEGER REFERENCES users(user_id),
--     group_id INTEGER REFERENCES groups(group_id)
-- );