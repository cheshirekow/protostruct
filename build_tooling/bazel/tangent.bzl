"""General utilities for bazel"""

def tangent_fetchobj(uuid, filename):
  """Download an artifact from openstack objecstore"""

  command = " ".join([
    "swift",
    "--os-auth-url=https://auth.cloud.ovh.net/v3/",
    "--os-identity-api-version=3",
    "--os-region-name=BHS",
    "--os-tenant-id=b5e0ef36abcb498b890d84b61555f063",
    "--os-tenant-name=0728979165260176",
    "--os-username=user-px7eRuuMs4hy",
    "--os-password=HHScMqa9DHaAphNqQx6YpaRsMVE4AakH",
    "download tangent-build",
    uuid,
    "-o $(RULEDIR)/{}".format(filename),
  ])
  native.genrule(
    name = "fetch-" + filename,
    outs = [filename],
    cmd = command,
    visibility = ["//visibility:public"],
  )
