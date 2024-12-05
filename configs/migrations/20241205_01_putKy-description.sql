-- Description
-- depends: 20241203_02_7lp51-add-events

GRANT CONNECT ON DATABASE "permission-db" TO db_admin;
GRANT USAGE ON SCHEMA public TO db_admin;
GRANT SELECT ON ALL TABLES IN SCHEMA public TO db_admin;
GRANT USAGE ON SCHEMA permission_app TO db_admin;
GRANT SELECT ON ALL TABLES IN SCHEMA permission_app TO db_admin;
GRANT USAGE, SELECT ON ALL SEQUENCES IN SCHEMA permission_app TO db_admin;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA permission_app TO db_admin;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA permission_app TO db_admin;
GRANT ALL PRIVILEGES ON ALL FUNCTIONS IN SCHEMA permission_app TO db_admin;
GRANT CREATE ON SCHEMA permission_app TO db_admin;
GRANT ALL PRIVILEGES ON SEQUENCE permission_app.permissions_user_permission_id_seq TO db_admin;
