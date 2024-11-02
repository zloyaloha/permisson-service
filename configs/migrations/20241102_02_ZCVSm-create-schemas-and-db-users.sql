-- Create schemas and db users
-- depends: 20241102_01_NRudT-create-users-and-group

CREATE USER db_admin WITH PASSWORD 'sap1234';
CREATE USER app_user WITH PASSWORD 'sup1234';

CREATE SCHEMA permission_app;
ALTER TABLE public.users SET SCHEMA permission_app;
ALTER TABLE public.groups SET SCHEMA permission_app;
ALTER TABLE public.user_to_group SET SCHEMA permission_app;

GRANT USAGE ON SCHEMA permission_app to app_user;
GRANT CONNECT ON DATABASE "permission-db" TO app_user;
GRANT ALL PRIVILEGES ON DATABASE "permission-db" TO db_admin;