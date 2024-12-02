-- Get file list with owner and group
-- depends: 20241119_02_ppdR0-create-file-func

CREATE OR REPLACE FUNCTION permission_app.GetFileTreeWithPermissions()
RETURNS TABLE (
    name VARCHAR(255),
    type file_type,
    path TEXT,
    permissions TEXT, -- Права доступа (rrrwwwxxx)
    owner_name VARCHAR(50),
    group_name VARCHAR(255)
)
LANGUAGE plpgsql
AS $$
BEGIN
    RETURN QUERY 
    WITH RECURSIVE file_tree AS (
        -- Начало с корневого узла
        SELECT 
            n.file_id,
            n.name,
            n.type,
            CAST(n.name AS TEXT) AS path,
            n.parent_id
        FROM permission_app.nodes n
        WHERE n.parent_id IS NULL
        
        UNION ALL
        
        -- Рекурсивное добавление дочерних узлов
        SELECT 
            n.file_id,
            n.name,
            n.type,
            CAST(ft.path || '/' || n.name AS TEXT) AS path,
            n.parent_id
        FROM permission_app.nodes n
        INNER JOIN file_tree ft ON n.parent_id = ft.file_id
    )
    -- Выбираем данные о правах доступа
    SELECT 
        ft.name AS name,
        ft.type AS type,
        ft.path AS path,
        -- Агрегируем права из всех таблиц
        CONCAT(
            CASE WHEN COALESCE(pu.can_read, FALSE) THEN 'r' ELSE '-' END,
            CASE WHEN COALESCE(pu.can_write, FALSE) THEN 'w' ELSE '-' END,
            CASE WHEN COALESCE(pu.can_execute, FALSE) THEN 'x' ELSE '-' END,
            CASE WHEN COALESCE(pg.can_read, FALSE) THEN 'r' ELSE '-' END,
            CASE WHEN COALESCE(pg.can_write, FALSE) THEN 'w' ELSE '-' END,
            CASE WHEN COALESCE(pg.can_execute, FALSE) THEN 'x' ELSE '-' END,
            CASE WHEN COALESCE(pa.can_read, FALSE) THEN 'r' ELSE '-' END,
            CASE WHEN COALESCE(pa.can_write, FALSE) THEN 'w' ELSE '-' END,
            CASE WHEN COALESCE(pa.can_execute, FALSE) THEN 'x' ELSE '-' END
        ) AS permissions,
        u.login AS owner_name, -- Имя владельца
        g.name AS group_name -- Имя группы владельцев
    FROM file_tree AS ft
    LEFT JOIN permission_app.permissions_user AS pu ON ft.file_id = pu.node_id
    LEFT JOIN permission_app.permissions_group AS pg ON ft.file_id = pg.node_id
    LEFT JOIN permission_app.permissions_all AS pa ON ft.file_id = pa.node_id
    LEFT JOIN permission_app.users AS u ON pu.user_id = u.user_id -- Владелец (пользователь)
    LEFT JOIN permission_app.groups AS g ON pg.group_id = g.group_id -- Владелец (группа)
    ORDER BY ft.path; -- Сортировка по пути
END;
$$;

-- Grant permissions to use the function
GRANT EXECUTE ON FUNCTION permission_app.GetFileTreeWithPermissions() TO app_user;