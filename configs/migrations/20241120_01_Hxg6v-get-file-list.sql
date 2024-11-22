-- Get file list
-- depends: 20241119_02_ppdR0-create-file-func

CREATE OR REPLACE FUNCTION permission_app.GetFileTreeWithPermissions()
RETURNS TABLE (
    name VARCHAR(255),
    type file_type, -- предполагаемый тип file_type
    path TEXT,
    can_read BOOLEAN,
    can_write BOOLEAN
)
LANGUAGE plpgsql
AS $$ 
BEGIN
    -- Начинаем выполнение запроса с рекурсивного CTE
    RETURN QUERY 
    WITH RECURSIVE file_tree AS (
        -- Начало с корневого узла
        SELECT 
            n.file_id,
            n.name, -- Переходим к типу VARCHAR(255) без преобразования
            n.type, -- Используем тип file_type как есть
            CAST(n.name AS TEXT) AS path, -- Приведение типа к TEXT
            n.parent_id
        FROM permission_app.nodes n
        WHERE n.parent_id IS NULL
        
        UNION ALL
        
        -- Рекурсивное добавление дочерних узлов
        SELECT 
            n.file_id,
            n.name, -- Переходим к типу VARCHAR(255) без преобразования
            n.type, -- Используем тип file_type как есть
            CAST(ft.path || '/' || n.name AS TEXT) AS path, -- Приведение типа к TEXT
            n.parent_id
        FROM permission_app.nodes n
        INNER JOIN file_tree ft ON n.parent_id = ft.file_id
    )
    -- Теперь выбираем нужные столбцы и получаем окончательные результаты
    SELECT 
        ft.name AS name,             -- Явно указываем имя
        ft.type AS type,             -- Явно указываем тип
        ft.path AS path,             -- Явно указываем путь
        p.can_read AS can_read,      -- Явно указываем разрешение на чтение
        p.can_write AS can_write     -- Явно указываем разрешение на запись
    FROM file_tree AS ft
    LEFT JOIN permission_app.permissions AS p ON ft.file_id = p.node_id  -- Связываем с таблицей разрешений
    ORDER BY ft.path;  -- Сортируем по пути
END;
$$;

-- Grant permissions to use the function
GRANT EXECUTE ON FUNCTION permission_app.GetFileTreeWithPermissions() TO app_user;
