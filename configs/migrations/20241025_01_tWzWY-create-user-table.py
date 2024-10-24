"""
Create user table
"""

from yoyo import step

__depends__ = {}

step("""
CREATE TABLE users (
--     user_id SERIAL PRIMARY KEY,
--     login VARCHAR(50) NOT NULL,
--     password VARCHAR(100) NOT NULL,
--     session_id INT,
--     time_registration TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
--     root BOOL
-- );
""", """
DROP TABLE users;
""")