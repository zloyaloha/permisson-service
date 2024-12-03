-- Get file id
-- depends: 20241117_01_sAk0i-file-table

CREATE OR REPLACE FUNCTION permission_app.FileId(fileName TEXT)
    RETURNS INTEGER
    LANGUAGE plpgsql
AS
$$
DECLARE
    id INTEGER;
BEGIN
    SELECT file_id
    INTO id
    FROM permission_app.nodes
    WHERE name = fileName
    LIMIT 1;

    IF id IS NULL THEN
        RETURN 0;
    ELSE
        RETURN id;
    END IF;
END;
$$;

GRANT EXECUTE ON FUNCTION permission_app.FileId(text) TO app_user;