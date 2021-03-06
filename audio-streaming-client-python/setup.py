from setuptools import setup, find_packages
# import os

# here = os.path.abspath(os.path.dirname(__file__))
# demo = open(os.path.join(here, 'client_demo.py')).read()

setup(
    name="baidu-acu-asr",
    version="1.0.7",
    description="asr grpc client",
    long_description="move product_id and enable_flush_data to be must params",
    author="Baidu",
    url="https://github.com/baidubce/pie/tree/master/audio-streaming-client-python-sdk",
    author_email="1908131339@qq.com",
    packages=find_packages(),
    license="Apache License",
    python_requires=">=2.7",
    install_requires=["protobuf", "grpcio"],
    keywords = ['baidu', 'asr', 'speech'],
)

