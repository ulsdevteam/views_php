<?php
// $Id$

/**
 * Caching plugin that provides cache control based on custom PHP code.
 *
 * @ingroup views_cache_plugins
 */
class views_php_plugin_cache extends views_plugin_cache {

  /**
   * Implements views_plugin_cache#summary_title().
   */
  function summary_title() {
    return t('PHP');
  }

  /**
   * Implements views_object#option_definition().
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['php_cache_results'] = array('default' => '');
    $options['php_cache_output'] = array('default' => '');

    return $options;
  }

  /**
   * Implements views_plugin#options_form().
   */
  function options_form(&$form, &$form_state) {
    $form['php_cache_results'] = array(
      '#type' => 'textarea',
      '#id' => drupal_html_id('edit-options-php-cache-results'),
      '#title' => t('Result cache code'),
      '#default_value' => $this->options['php_cache_results'],
      '#rows' => 5,
      '#description' => t('The code must return TRUE if the cache is still fresh, FALSE otherwise.') . ' <strong>' . t('Do not use &lt;?php ?&gt; delimiters.') . '</strong>',
    );
    views_php_check_input_access($form['php_cache_output']);
    $form['php_cache_results_variables'] = views_php_available_variables($form['php_cache_results'], $this->view, $this, array(
      '$view',
      '$plugin' => t('The plugin object.'),
      '$cache' => t('The cache object.'),
    ));
    $form['php_cache_output'] = array(
      '#type' => 'textarea',
      '#id' => drupal_html_id('edit-options-php-cache-output'),
      '#title' => t('Output cache code'),
      '#default_value' => $this->options['php_cache_output'],
      '#rows' => 5,
      '#description' => t('The code must return TRUE if the cache is still fresh, FALSE otherwise.') . ' <strong>' . t('Do not use &lt;?php ?&gt; delimiters.') . '</strong>',
    );
    views_php_check_input_access($form['php_cache_output']);
    $form['php_cache_output_variables'] = views_php_available_variables($form['php_cache_output'], $this->view, $this, array(
      '$view',
      '$plugin' => t('The plugin object.'),
      '$cache' => t('The cache object.'),
    ));
  }

  /**
   * Implements views_plugin_cache#cache_get()
   */
  function cache_get($type) {
    //$cutoff = $this->cache_expire($type);
    switch ($type) {
      case 'query':
        // Not supported currently, but this is certainly where we'd put it.
        return FALSE;
      case 'results':
        $cache = cache_get($this->get_results_key(), $this->table);
        $fresh = !empty($cache);
        if ($fresh && !empty($this->options['php_cache_results'])) {
          $lamda_function = create_function('$view, $plugin, $cache', $this->options['php_cache_results']);
          ob_start();
          $fresh = $lamda_function($this->view, $this, $cache);
          ob_end_clean();
        }
        // Values to set: $view->result, $view->total_rows, $view->execute_time,
        // $view->current_page.
        if ($fresh) {
          //if (!$cutoff || $cache->created > $cutoff) {
            $this->view->result = $cache->data['result'];
            $this->view->total_rows = $cache->data['total_rows'];
            $this->view->set_current_page = $cache->data['current_page'];
            $this->view->execute_time = 0;
            return TRUE;
          //}
        }
        return FALSE;
      case 'output':
        $cache = cache_get($this->get_output_key(), $this->table);
        $fresh = !empty($cache);
        if ($fresh && !empty($this->options['php_cache_output'])) {
          $lamda_function = create_function('$view, $plugin, $cache', $this->options['php_cache_output']);
          ob_start();
          $fresh = $lamda_function($this->view, $this, $cache);
          ob_end_clean();
        }
        if ($fresh) {
          //if (!$cutoff || $cache->created > $cutoff) {
            $this->storage = $cache->data;
            $this->view->display_handler->output = $cache->data['output'];
            $this->restore_headers();
            return TRUE;
          //}
        }
        return FALSE;
    }
  }
}
