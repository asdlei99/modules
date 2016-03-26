void my_json()
{
    QString http_response = "{\"key\":\"a99fdd865c2-10000\",\"id\":20160324,\"sign\":\"f0dd9e5226d0e77\"}";

    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("value = "+ http_response);

    QScriptValueIterator it(sc);
    while(it.hasNext()) {
        it.next();
        qDebug() << it.name() << ": "<< it.value().toString();
    }

    QScriptValueIterator _it(sc);
    while(_it.hasNext()) {
        _it.next();
        if(_it.name() == "key")
            qDebug() << "¹ýÂË×Ö¶Î£º" << _it.name() << _it.value().toString();
    }
}