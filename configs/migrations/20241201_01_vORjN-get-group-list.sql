-- Get group list
-- depends: 20241130_03_kIAu5-get-users-list

CREATE OR REPLACE FUNCTION permission_app.GetGroupList()
RETURNS TABLE (
    group_name VARCHAR,
    user_list TEXT
) AS $$
BEGIN
    RETURN QUERY
    SELECT 
        g.name AS group_name,
        STRING_AGG(u.login, ',') AS user_list
    FROM 
        permission_app.groups g
    LEFT JOIN 
        permission_app.user_to_group ug ON g.group_id = ug.group_id
    LEFT JOIN 
        permission_app.users u ON ug.user_id = u.user_id
    GROUP BY 
        g.group_id, g.name
    ORDER BY 
        g.name;
END;
$$ LANGUAGE plpgsql SECURITY DEFINER;


GRANT EXECUTE ON FUNCTION permission_app.GetGroupList() TO app_user;