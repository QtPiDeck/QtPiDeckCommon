#will be called from server/client script
def getCommonList(args):
    base = ["boost-test"]
    add = {"qt6.1.0": "openssl"}
    return base + [add[arg] for arg in filter(lambda x: x in add, args)]