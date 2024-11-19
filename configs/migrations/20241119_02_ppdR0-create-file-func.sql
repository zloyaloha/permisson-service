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
    parent_id_find INT := NULL;     -- ID родительского узла
    current_id INT;            -- ID текущего узла
    part TEXT;                 -- Текущая часть пути
    new_file_id INT;           -- ID нового файла
    new_file_permission_id INT; -- ID разрешения
    username_user_id INT;               -- ID пользователя, который добавляет файл
BEGIN

    SELECT user_id INTO username_user_id
    FROM permission_app.users
    WHERE login = username;

    -- Разбиваем путь на части
    parts := string_to_array(path_string, '/');

    -- Проверяем существование всех узлов в пути
    FOREACH part IN ARRAY parts LOOP
        SELECT file_id INTO current_id
        FROM permission_app.nodes
        WHERE name = part AND type = 'DIR';

        -- Если узел не найден, прекращаем выполнение
        IF current_id IS NULL THEN
            RAISE NOTICE 'Path % does not exist. Nothing was created.', part;
            RETURN 'Path Error';
        END IF;

        -- Переходим на следующий уровень
        parent_id_find := current_id;
    END LOOP;

    -- Проверяем, существует ли файл с таким именем в указанной директории
    SELECT file_id INTO current_id
    FROM permission_app.nodes
    WHERE name = file_name AND parent_id = parent_id_find;

    -- Если файла нет, создаем его
    IF current_id IS NULL THEN
        INSERT INTO permission_app.nodes (parent_id, name, type, file_data)
        VALUES (parent_id_find, file_name, 'FILE', file_data);

        SELECT file_id INTO new_file_id
        FROM permission_app.nodes
        WHERE name = file_name;

        INSERT INTO permission_app.permissions(node_id, user_id, can_read, can_write)
        VALUES (new_file_id, username_user_id, TRUE, TRUE);

        INSERT INTO permission_app.events(user_id, file_id, event)
        VALUES (username_user_id, new_file_id, 'CREATE_FILE');
    ELSE
        RETURN 'File exists';
    END IF;
    RETURN file_name;
END;
$$;

GRANT EXECUTE ON FUNCTION permission_app.AddFileToPath(TEXT, TEXT, TEXT, BYTEA) TO app_user;
