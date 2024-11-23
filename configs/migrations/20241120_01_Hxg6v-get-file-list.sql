-- Get file list with owner and group
-- depends: 20241119_02_ppdR0-create-file-func

CREATE OR REPLACE FUNCTION permission_app.GetFileTreeWithPermissions()
RETURNS TABLE (
    name VARCHAR(255),
    type file_type,  -- Тип файла
    path TEXT,       -- Полный путь
    can_read BOOLEAN,
    can_write BOOLEAN,
    can_exec BOOLEAN,
    owner_name TEXT, -- Имя владельца файла
    group_name TEXT  -- Имя группы владельцев файла
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
    -- Теперь выбираем нужные столбцы и объединяем с таблицей permissions для получения владельца и группы
    SELECT 
        ft.name AS name,
        ft.type AS type,
        ft.path AS path,
        p.can_read AS can_read,
        p.can_write AS can_write,
        p.can_execute AS can_exec,
        o.name AS owner_name,      -- Имя владельца
        g.name AS group_name       -- Имя группы владельцев
    FROM file_tree AS ft
    LEFT JOIN permission_app.permissions AS p ON ft.file_id = p.node_id
    LEFT JOIN permission_app.users AS o ON p.user_id = o.user_id -- Присоединяем информацию о владельце
    LEFT JOIN permission_app.groups AS g ON p.group_id = g.group_id -- Присоединяем информацию о группе
    ORDER BY ft.path;  -- Сортируем по пути
END;
$$;

-- Grant permissions to use the function
GRANT EXECUTE ON FUNCTION permission_app.GetFileTreeWithPermissions() TO app_user;
