async def test_get_index(aclient):
    response = await aclient.get("/")

    assert response.status_code == 200
    assert response.json() == {"message": "Hello World"}
