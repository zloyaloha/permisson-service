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
    user_group_id INT;       -- ID группы родительской директории
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
        WHERE name = part AND type = 'DIR' AND (parent_id = parent_id_find OR parent_id IS NULL);

        IF current_id IS NULL THEN
            RAISE NOTICE 'Path % does not exist. Nothing was created.', part;
            RETURN 'Path Error';
        END IF;

        parent_id_find := current_id;
    END LOOP;

    -- Получаем ID группы родительской директории
    SELECT group_id INTO user_group_id
    FROM permission_app.groups
    WHERE name = (username || '_group');

    -- Проверяем, существует ли директория с таким именем в указанной директории
    SELECT file_id INTO current_id
    FROM permission_app.nodes
    WHERE name = dir_name AND parent_id = parent_id_find;

    IF current_id IS NULL THEN
        -- Вставляем новую директорию
        INSERT INTO permission_app.nodes (parent_id, name, type)
        VALUES (parent_id_find, dir_name, 'DIR')
        RETURNING file_id INTO new_dir_id;

        -- Устанавливаем права для новой директории
        INSERT INTO permission_app.permissions_user (node_id, user_id)
        VALUES (new_dir_id, username_user_id);

        INSERT INTO permission_app.permissions_group(node_id, group_id)
        VALUES (new_dir_id, user_group_id);

        INSERT INTO permission_app.permissions_all(node_id)
        VALUES (new_dir_id);

        -- Логируем событие создания директории
        INSERT INTO permission_app.events (user_id, file_id, event)
        VALUES (username_user_id, new_dir_id, 'CREATE_DIR');
    ELSE
        RETURN 'File exists';
    END IF;

    RETURN 'Success';
END;
$$;

GRANT EXECUTE ON FUNCTION permission_app.AddDirectoryToPath(TEXT, TEXT, TEXT) TO app_user;
