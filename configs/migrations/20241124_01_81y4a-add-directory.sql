-- Add directory
-- depends: 20241120_01_Hxg6v-get-file-list

CREATE OR REPLACE FUNCTION permission_app.AddDirectoryToPath(
    path_string TEXT, 
    dir_name TEXT, 
    username TEXT
)
RETURNS TEXT
LANGUAGE plpgsql
AS $$
DECLARE
    parts TEXT[];              -- Массив частей пути
    parent_id_find INT := NULL;-- ID родительского узла
    current_id INT;            -- ID текущего узла
    part TEXT;                 -- Текущая часть пути
    new_dir_id INT;            -- ID новой директории
    username_user_id INT;      -- ID пользователя, который добавляет директорию
BEGIN
    -- Получаем user_id пользователя
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

    -- Проверяем, существует ли директория с таким именем в указанной директории
    SELECT file_id INTO current_id
    FROM permission_app.nodes
    WHERE name = dir_name AND parent_id = parent_id_find AND type = 'DIR';

    -- Если директории нет, создаем её
    IF current_id IS NULL THEN
        INSERT INTO permission_app.nodes (parent_id, name, type)
        VALUES (parent_id_find, dir_name, 'DIR');

        -- Получаем ID созданной директории
        SELECT file_id INTO new_dir_id
        FROM permission_app.nodes
        WHERE name = dir_name AND parent_id = parent_id_find AND type = 'DIR';

        -- Добавляем права для текущего пользователя
        INSERT INTO permission_app.permissions(node_id, user_id, can_read, can_write)
        VALUES (new_dir_id, username_user_id, TRUE, TRUE);

        -- Логируем событие создания директории
        INSERT INTO permission_app.events(user_id, file_id, event)
        VALUES (username_user_id, new_dir_id, 'CREATE_FILE');
    ELSE
        RETURN 'Directory exists';
    END IF;

    RETURN 'Directory created: ' || dir_name;
END;
$$;

GRANT EXECUTE ON FUNCTION permission_app.AddDirectoryToPath(TEXT, TEXT, TEXT) TO app_user;
