std::vector<Object> convertQuadsToTriangles(std::vector<Object> objs) {
    for (Object& obj : objs) {
        std::vector<triangle> triangleTriangles;
        for (int i = 0; i < obj.tris.size(); i++) {
            // Create first triangle from first, second, and fourth vertices of the quad
            triangleTriangles.push_back(triangle(obj.tris[i].verts[0], obj.tris[i].verts[1], obj.tris[i].verts[3]));

            // Create second triangle from second, third, and fourth vertices of the quad
            triangleTriangles.push_back(triangle(obj.tris[i].verts[1], obj.tris[i].verts[2], obj.tris[i].verts[3]));
        }
        obj.tris = triangleTriangles;
    }
    return objs;
}