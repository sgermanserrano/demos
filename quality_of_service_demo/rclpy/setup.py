from setuptools import setup

package_name = 'quality_of_service_demo_py'

setup(
    name=package_name,
    version='0.8.0',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    author='Emerson Knapp',
    maintainer='Amazon ROS Contributions',
    maintainer_email='ros-contributions@amazon.com',
    keywords=['ROS'],
    classifiers=[
        'Intended Audience :: Developers',
        'License :: OSI Approved :: Apache Software License',
        'Programming Language :: Python',
        'Topic :: Software Development',
    ],
    description='Python nodes to demonstrate ROS2 QoS policies.',
    license='Apache License, Version 2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'lifespan = quality_of_service_demo_py.lifespan:main',
            'liveliness = quality_of_service_demo_py.liveliness:main',
            'deadline = quality_of_service_demo_py.deadline:main',
        ],
    },
)
