<?php
// $Id$

/**
 * A handler to filter a view using PHP defined by the administrator.
 *
 * @ingroup views_filter_handlers
 */
class views_php_handler_filter extends views_handler_filter {

  /**
   * Implements views_object#admin_summary().
   */
  function admin_summary() {
    return t('PHP');
  }

  /**
   * Implements views_object#option_definition().
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['php_setup'] = array('default' => '');
    $options['php_filter'] = array('default' => '');
    return $options;
  }

  /**
   * Implements views_handler#option_definition().
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['php_setup'] = array(
      '#type' => 'textarea',
      '#id' => drupal_html_id('edit-options-php-setup'),
      '#title' => t('Setup code'),
      '#default_value' => $this->options['php_setup'],
      '#rows' => 5,
      '#description' => t('Code to run right before execution of the view. May be useful to define functions to be re-used in the value and/or output code.') . ' <strong>' . t('Do not include &lt;?php ?&gt; delimiters to enclose PHP code.') . '</strong>',
    );
    views_php_check_input_access($form['php_setup']);
    $form['php_setup_variables'] = views_php_available_variables($form['php_setup'], $this->view, $this, array('$view', '$handler', '$static'));
    $form['php_filter'] = array(
      '#type' => 'textarea',
      '#id' => drupal_html_id('edit-options-php-filter'),
      '#title' => t('Filter code'),
      '#default_value' => $this->options['php_filter'],
      '#rows' => 5,
      '#description' => t('If the code returns TRUE the current row is removed from the results.') . ' <strong>' . t('Do not use &lt;?php ?&gt; delimiters to enclose PHP code.') . '</strong>',
    );
    views_php_check_input_access($form['php_filter']);
    $form['php_filter_variables'] = views_php_available_variables($form['php_filter'], $this->view, $this, array('$view', '$handler', '$static, $data'));
  }

  /**
   * Implements views_handler_filter#query().
   */
  function query() {
    // Inform views_php_views_pre_execute() to seize control over the query.
    $this->view->views_php = TRUE;
  }

  /**
   *
   * @see views_php_views_pre_execute()
   */
  function php_pre_execute() {
    // Ecexute static PHP code.
    if (!empty($this->options['php_setup'])) {
      $lamda_function = create_function('$view, $handler, &$static', $this->options['php_setup']);
      ob_start();
      $lamda_function($this->view, $this, &$this->php_static_variable);
      ob_end_clean();
    }
  }

  /**
   *
   * @see views_php_views_post_execute()
   */
  function php_post_execute() {
    // Evaluate the PHP code.
    if (!empty($this->options['php_filter'])) {
      $lamda_function = create_function('$view, $handler, &$static, $data', $this->options['php_filter']);
      ob_start();
      foreach ($this->view->result as $i => $row) {
        if ($lamda_function($this->view, $this, &$this->php_static_variable, $row)) {
          unset($this->view->result[$i]);
        }
      }
      ob_end_clean();
    }
  }
}
