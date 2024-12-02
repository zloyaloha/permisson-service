-- Create file func
-- depends: 20241119_01_ZmQKh-permissions-table

CREATE OR REPLACE FUNCTION permission_app.AddFileToPath(
    path_string TEXT, 
    file_name TEXT, 
    username TEXT,
    file_data BYTEA DEFAULT NULL
)
RETURNS TEXT
LANGUAGE plpgsql
AS $$
DECLARE
    parts TEXT[];              -- Массив частей пути
    parent_id_find INT := NULL; -- ID родительского узла
    current_id INT;            -- ID текущего узла
    part TEXT;                 -- Текущая часть пути
    new_file_id INT;           -- ID нового файла
    username_user_id INT;      -- ID пользователя, который добавляет файл
    user_group_id INT;       -- ID группы родительской директории
BEGIN
    -- Получаем ID пользователя
    SELECT user_id INTO username_user_id
    FROM permission_app.users
    WHERE login = username;

    IF username_user_id IS NULL THEN
        RAISE EXCEPTION 'User not found';
    END IF;

    -- Разбиваем путь на части
    parts := string_to_array(path_string, '/');

    -- Проверяем существование всех узлов в пути
    FOREACH part IN ARRAY parts LOOP
        SELECT n.file_id INTO current_id
        FROM permission_app.nodes n
        WHERE name = part AND type = 'DIR' AND (parent_id = parent_id_find OR parent_id IS NULL);

        -- Если узел не найден, прекращаем выполнение
        IF current_id IS NULL THEN
            RAISE NOTICE 'Path % does not exist. Nothing was created.', part;
            RETURN 'Path Error';
        END IF;

        -- Переходим на следующий уровень
        parent_id_find := current_id;
    END LOOP;

    SELECT group_id INTO user_group_id
    FROM permission_app.groups
    WHERE name = (username|| '_group');

    -- Проверяем, существует ли файл с таким именем в указанной директории
    SELECT file_id INTO current_id
    FROM permission_app.nodes
    WHERE name = file_name AND parent_id = parent_id_find;

    -- Если файла нет, создаем его
    IF current_id IS NULL THEN
        -- Вставляем новый файл
        INSERT INTO permission_app.nodes (parent_id, name, type, file_data)
        VALUES (parent_id_find, file_name, 'FILE', file_data)
        RETURNING file_id INTO new_file_id;

        -- Устанавливаем права для нового файла
        INSERT INTO permission_app.permissions_user (node_id, user_id)
        VALUES (new_file_id, username_user_id);

        IF user_group_id IS NOT NULL THEN
            INSERT INTO permission_app.permissions_group (node_id, group_id)
            VALUES (new_file_id, user_group_id);
        END IF;

        INSERT INTO permission_app.permissions_all (node_id)
        VALUES (new_file_id);

        -- Записываем событие создания файла
        INSERT INTO permission_app.events (user_id, file_id, event)
        VALUES (username_user_id, new_file_id, 'CREATE_FILE');
    ELSE
        RETURN 'File exists';
    END IF;

    RETURN 'Success';
END;
$$;

-- Предоставляем права на выполнение функции
GRANT EXECUTE ON FUNCTION permission_app.AddFileToPath(TEXT, TEXT, TEXT, BYTEA) TO app_user;

