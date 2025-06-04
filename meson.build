project('DijikstrasPorject', 'c',
  version: '0.1.0',
  meson_version: '>= 0.50.0',
  default_options: [
    'warning_level=2',
    'buildtype=debugoptimized',
    'c_std=c11'
  ]
)

includes = include_directories('include')

gnome = import('gnome')

resources = gnome.compile_resources('resoures', 'dijkstrasProject.gresource.xml',
  source_dir: 'data')

deps =[    dependency('gtkmm-4.0'),
  dependency('glib-2.0'),
  dependency('gio-2.0'),
  dependency('gdkmm-4.0'),
  dependency('cairo'),
  dependency('cairo-gobject')
]

c_sources = [
  'src/main.c',
  'src/main.h',
  'src/osm_reader.c',
  'src/osm_reader.h',
  'src/dijkstra.c',
  'src/graph.c',
  'src/graph.h',
  'src/dijkstra.h'
]
executable(
    meson.project_name(),
    c_sources,
    resources,
    include_directories: includes,
    dependencies: deps,
    win_subsystem: 'windows'
)
