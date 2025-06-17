project('DijikstrasProject', 'c',
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

deps = [
  dependency('gtk+-3.0'),
  dependency('glib-2.0'),
  dependency('gio-2.0'),
  dependency('cairo'),
  dependency('cairo-gobject'),
  dependency('libxml-2.0'),
  meson.get_compiler('c').find_library('m', required: true)
]

c_sources = [
  'src/main.c',
  'src/osm_reader.c',
  'src/dijkstra.c',
  'src/graph.c',
  'src/edit.c',
  'src/ui_helpers.c',
  'src/interaction.c',
  'src/drawing.c',
  'src/callbacks.c'
]
executable(
    meson.project_name(),
    c_sources,
    resources,
    include_directories: includes,
    dependencies: deps
)
